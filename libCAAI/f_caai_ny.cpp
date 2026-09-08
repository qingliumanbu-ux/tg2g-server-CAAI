/*=========================================================================
//程序名称:     f_caai_ny
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:     
//修改日期:  
//函数说明：能源特殊方式分摊，根据具体的要求
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件
int f_caai_rate(CString stats_period, CString sub_backlog_code, CString divvy_type, CDbConnection * conn);
double f_caai_getprice(const CString& mat_code, const CString& company_code, const CString& price_terms, const CString& dept_code, CDbConnection * conn);
BM2_FUNCTION_EXPORT
int f_caai_ny(CString stats_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;	
	CDecimal price = 0;
	CString sqlstr = "", sqlstr_sub = "", begin_time = "", end_time = "", divvy_type="1";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");	
	CDecimal all_wt, ft_wt, use_all_wt,use_all_amt;

	CModel tcaaia12("TCAAIA12");

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	try
	{

		//产出
		sqlstr = " delete from tcaai02"
			" where 1=1"
			" AND RULE_TYPE in ( '4','5')"
			" AND DEPT_CODE like trim(@dept_code)||'%'"
			" AND STATS_PERIOD = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		sqlstr = " SELECT S_DATETIME,E_DATETIME FROM tcaac06 WHERE STATS_PERIOD = @stats_period AND	VALID_FLAG	=	'1'";
		
		cmd_inq.SetCommandText(sqlstr);// 设置执行的SQL语句
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			begin_time = cmd_inq.GetString(1).SubstringNE(0, 8);
			end_time = cmd_inq.GetString(2).SubstringNE(0, 8);
		}


		//针对需分摊量的，按照分摊系数来核算
		sqlstr = "select *"
			" from tcaaia12 "
			" where 1=1"
			" AND busi_type != 'SC'"
			" AND DEPT_CODE =@dept_code "
			" AND PROD_DATE >= @begin_time "
			" and PROD_DATE<=@end_time "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("begin_time", begin_time);
		cmd_inq.Parameters.Set("end_time", end_time);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tcaaia12);
			//根据工序及物料代码取分摊规则
			sqlstr = " select MEASURE_MODE "
				" FROM TCAAC07 "
				" WHERE SUB_BACKLOG_CODE =@sub_backlog_code "
				" AND MAT_CODE = @mat_code "
				;
			divvy_type = "1";
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("sub_backlog_code", tcaaia12["SUB_BACKLOG_CODE"]);
			cmd_inq_1.Parameters.Set("mat_code", tcaaia12["MAT_CODE"]);
			cmd_inq_1.ExecuteReader();
			while (cmd_inq_1.Read())
			{				
				divvy_type = cmd_inq_1.GetString(1);
			}
			cmd_inq_1.Close();

			if (divvy_type == "7")
			{
				//删除tcaai01表按系数分摊基础数据
				sqlstr = " delete from tcaai01"
					" where 1=1"
					" AND sub_backlog_code = @sub_backlog_code"
					" and divvy_type = '7'"
					" and STATS_PERIOD = @stats_period"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("sub_backlog_code", tcaaia12["SUB_BACKLOG_CODE"]);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();

				//往tcaai01表插入物料对应产品规格牌号的产出数据
// DM8 适配 CHANGE-254:写入/查询 TCAAI01。空串搜索 DECODE 改为标准 CASE。
// 改写原因：空串搜索 DECODE(x,'',a,b) 改为标准 CASE WHEN x IS NULL OR x='' THEN a ELSE b,与 CHANGE-107 同理,不依赖空串/NULL 匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
				// sqlstr = " insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, product_code, product_code_cname,divvy_type, divvy_basic_n ) "
					// " SELECT @datenow,  t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.MAT_CODE,t1.mat_name,'7',SUM(DECODE(CAL_RATE,'',0,CAL_RATE)*WT) "
					// " FROM TCAAIS1 t1 "
					// " left join tcaac09 t9 "
					// " on t1.sg_sign = t9.sg_sign AND t1.mat_thick = t9.mat_thick  AND t1.mat_width = t9.mat_width and t1.cost_center=t9.sub_backlog_code and t1.dept_code=t9.dept_code and t9.mat_code=@mat_code "
					// " WHERE 1=1 "
					// " AND t1.sub_backlog_code = @sub_backlog_code "
					// " AND PRO_FLAG ='O' " //产出工序
					// " AND stats_period = @stats_period "
					// " group by   t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.MAT_CODE,t1.mat_name "
					// " HAVING SUM(DECODE(CAL_RATE,'',0,CAL_RATE)*WT)!=0 "
					// ;
// DM8 SQL：
				sqlstr = " insert into tcaai01(REC_CREATE_TIME,dept_code, stats_period, cost_center, prod_date, prod_shift_group, prod_shift_no, sub_backlog_code, equ_no, sg_sign, mat_thick, mat_width, product_code, product_code_cname,divvy_type, divvy_basic_n ) "
					" SELECT @datenow,  t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.MAT_CODE,t1.mat_name,'7',SUM(CASE WHEN CAL_RATE IS NULL OR CAL_RATE = '' THEN 0 ELSE CAL_RATE END*WT) "
					" FROM TCAAIS1 t1 "
					" left join tcaac09 t9 "
					" on t1.sg_sign = t9.sg_sign AND t1.mat_thick = t9.mat_thick  AND t1.mat_width = t9.mat_width and t1.cost_center=t9.sub_backlog_code and t1.dept_code=t9.dept_code and t9.mat_code=@mat_code "
					" WHERE 1=1 "
					" AND t1.sub_backlog_code = @sub_backlog_code "
					" AND PRO_FLAG ='O' " //产出工序
					" AND stats_period = @stats_period "
					" group by   t1.DEPT_CODE, STATS_PERIOD, COST_CENTER, prod_date, prod_shift_group, prod_shift_no, t1.sub_backlog_code, equ_no, t1.sg_sign, t1.mat_thick, t1.mat_width,t1.MAT_CODE,t1.mat_name "
					" HAVING SUM(CASE WHEN CAL_RATE IS NULL OR CAL_RATE = '' THEN 0 ELSE CAL_RATE END*WT)!=0 "
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("datenow", datenow);
				cmd_inq_1.Parameters.Set("mat_code", tcaaia12["MAT_CODE"]);
				cmd_inq_1.Parameters.Set("sub_backlog_code", tcaaia12["SUB_BACKLOG_CODE"]);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();
			}


			//取总消耗量
			sqlstr_sub = "";
			sqlstr = " select sum(DIVVY_BASIC_N)"
				" from tcaai01"
				" where 1=1"
				;
			if (tcaaia12["PROD_DATE"].ToString().Trim() != "")
			{
				sqlstr_sub = sqlstr_sub + " AND PROD_DATE =@prod_date ";
			}
			if (tcaaia12["SHIFT_GROUP"].ToString().Trim() != "")
			{
				sqlstr_sub = sqlstr_sub + " AND PROD_SHIFT_GROUP =@prod_shift_group ";
			}
			if (tcaaia12["SHIFT_NO"].ToString().Trim() != "")
			{
				sqlstr_sub = sqlstr_sub + " AND PROD_SHIFT_NO =@prod_shift_no ";
			}
			if (tcaaia12["EQU_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND equ_no =@equ_no ";
			}
			sqlstr = sqlstr+sqlstr_sub + " and DIVVY_TYPE = @divvy_type"
				" and dept_code = @dept_code"
				" and cost_center = @cost_center"
				" AND STATS_PERIOD = @stats_period"
				;
			all_wt = 0;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("datenow", datenow);
			cmd_inq_1.Parameters.Set("divvy_type", divvy_type);
			cmd_inq_1.Parameters.Set("use_wt", tcaaia12["COMSUME_WT"]);
			cmd_inq_1.Parameters.Set("stats_period", stats_period);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("cost_center", tcaaia12["SUB_BACKLOG_CODE"]);
			cmd_inq_1.Parameters.Set("prod_date", tcaaia12["PROD_DATE"]);
			cmd_inq_1.Parameters.Set("prod_shift_group", tcaaia12["SHIFT_GROUP"]);
			cmd_inq_1.Parameters.Set("prod_shift_no", tcaaia12["SHIFT_NO"]);
			cmd_inq_1.Parameters.Set("equ_no", tcaaia12["EQU_NO"]);
			all_wt = cmd_inq_1.ExecuteScalar();
			cmd_inq_1.Close();

			//插入值
			
			sqlstr =" insert into tcaai02 (REC_CREATE_TIME,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_code,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,MAT_CODE,RULE_TYPE,DIVVY_TYPE,WT,vfree1,vfree2,vfree3,vfree4,vfree5)"
				" select @datenow,dept_code,prod_date,sub_backlog_code,cost_center,stats_period,product_CODE,equ_no, sg_sign,prod_shift_group,prod_shift_no, mat_thick, mat_width,@mat_code,'4',DIVVY_TYPE,decode(@all_wt,0,0,ROUND(DIVVY_BASIC_N/@all_wt*@use_wt,4)) ,vfree1,vfree2,vfree3,vfree4,vfree5" 
				" from tcaai01"
				" where 1=1"
				;			

			sqlstr = sqlstr+ sqlstr_sub + " and DIVVY_TYPE = @divvy_type"
				" and dept_code = @dept_code"
				" and cost_center = @cost_center"				
				" AND STATS_PERIOD = @stats_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);

			//Log::Trace("", "", "sqlstr123={0}", sqlstr);
			cmd_inq_1.Parameters.Set("datenow", datenow);
			cmd_inq_1.Parameters.Set("mat_code", tcaaia12["MAT_CODE"]);
			cmd_inq_1.Parameters.Set("divvy_type", divvy_type);
			cmd_inq_1.Parameters.Set("use_wt", tcaaia12["COMSUME_WT"]);
			cmd_inq_1.Parameters.Set("all_wt", all_wt);
			cmd_inq_1.Parameters.Set("stats_period", stats_period);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("cost_center", tcaaia12["SUB_BACKLOG_CODE"]);
			cmd_inq_1.Parameters.Set("prod_date", tcaaia12["PROD_DATE"]);
			cmd_inq_1.Parameters.Set("prod_shift_group", tcaaia12["SHIFT_GROUP"]);
			cmd_inq_1.Parameters.Set("prod_shift_no", tcaaia12["SHIFT_NO"]);
			cmd_inq_1.Parameters.Set("equ_no", tcaaia12["EQU_NO"]);
			cmd_inq_1.ExecuteNonQuery();
			cmd_inq_1.Close();

			//误差调整
			sqlstr = "select sum(WT),SUM(AMT)"
				" from tcaai02"
				" where 1=1"
				;
			if (tcaaia12["EQU_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND EQU_NO =@equ_no ";
			}
			sqlstr = sqlstr+sqlstr_sub + " AND MAT_CODE = @mat_code"
				" AND RULE_TYPE ='4' "
				" and DIVVY_TYPE = @divvy_type"
				" and dept_code = @dept_code"
				" and cost_center = @cost_center"
				" AND STATS_PERIOD = @stats_period"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			//Log::Trace("", "", "sqlstr={0}", sqlstr);
			cmd_inq_1.Parameters.Set("mat_code", tcaaia12["MAT_CODE"]);
			cmd_inq_1.Parameters.Set("stats_period", stats_period);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("divvy_type", divvy_type);
			cmd_inq_1.Parameters.Set("cost_center", tcaaia12["SUB_BACKLOG_CODE"]);
			cmd_inq_1.Parameters.Set("prod_date", tcaaia12["PROD_DATE"]);
			cmd_inq_1.Parameters.Set("prod_shift_group", tcaaia12["SHIFT_GROUP"]);
			cmd_inq_1.Parameters.Set("prod_shift_no", tcaaia12["SHIFT_NO"]);
			cmd_inq_1.Parameters.Set("equ_no", tcaaia12["EQU_NO"]);
			cmd_inq_1.ExecuteReader();
			while (cmd_inq_1.Read())
			{
				use_all_wt = cmd_inq_1.GetDecimal(1);
				use_all_amt = cmd_inq_1.GetDecimal(2);
			}
			cmd_inq_1.Close();

			sqlstr = "update tcaai02 "
				" set wt=wt+@use_wt"
				" where 1=1"
				;			
			sqlstr = sqlstr+sqlstr_sub + " and wt in (select max(wt) from tcaai02 where 1=1" + sqlstr_sub + " and mat_code =@mat_code and dept_code = @dept_code and cost_center = @cost_center AND RULE_TYPE ='4'  and DIVVY_TYPE =@divvy_type AND stats_period = @stats_period)"
					" AND MAT_CODE = @mat_code"
					" AND RULE_TYPE ='4' "
					" and DIVVY_TYPE = @divvy_type"
					" and dept_code = @dept_code"
					" and cost_center = @cost_center"
					" AND STATS_PERIOD = @stats_period"
					" and rownum=1"
					;
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("use_wt", tcaaia12["COMSUME_WT"] - use_all_wt);
				cmd_inq_1.Parameters.Set("mat_code", tcaaia12["MAT_CODE"]);
				cmd_inq_1.Parameters.Set("divvy_type", divvy_type);
				cmd_inq_1.Parameters.Set("stats_period", stats_period);
				cmd_inq_1.Parameters.Set("dept_code", dept_code);
				cmd_inq_1.Parameters.Set("cost_center", tcaaia12["SUB_BACKLOG_CODE"]);
				cmd_inq_1.Parameters.Set("prod_date", tcaaia12["PROD_DATE"]);
				cmd_inq_1.Parameters.Set("prod_shift_group", tcaaia12["SHIFT_GROUP"]);
				cmd_inq_1.Parameters.Set("prod_shift_no", tcaaia12["SHIFT_NO"]);
				cmd_inq_1.Parameters.Set("equ_no", tcaaia12["EQU_NO"]);
				cmd_inq_1.ExecuteNonQuery();
				cmd_inq_1.Close();			
		}
		cmd_inq.Close();

		////价格数据
		//sqlstr = "select distinct mat_code "
		//	" from tcaaia12 "
		//	" where 1=1"
		//	" AND DEPT_CODE =@dept_code "
		//	" AND PROD_DATE >= @begin_time "
		//	" and PROD_DATE<=@end_time "
		//	;
		//cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("dept_code", dept_code);
		//cmd_inq.Parameters.Set("begin_time", begin_time);
		//cmd_inq.Parameters.Set("end_time", end_time);
		//cmd_inq.ExecuteReader();
		//while (cmd_inq.Read())
		//{
		//	price = f_caai_getprice(cmd_inq.GetString(1)," ", "PJ", dept_code, conn);

		//	sqlstr = " update tcaai02 set amt=wt*@price,unit_price=@price "
		//		" WHERE 1=1"
		//		" and mat_code=@mat_code"
		//		" AND RULE_TYPE = '4'"
		//		" AND DEPT_CODE like trim(@dept_code)||'%'"
		//		" AND STATS_PERIOD = @stats_period"
		//		;
		//	cmd_inq_1.SetCommandText(sqlstr);
		//	cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(1));
		//	cmd_inq_1.Parameters.Set("price", price);
		//	cmd_inq_1.Parameters.Set("dept_code", dept_code);
		//	cmd_inq_1.Parameters.Set("stats_period", stats_period);
		//	cmd_inq_1.ExecuteNonQuery();
		//	cmd_inq_1.Close();
		//}
		//cmd_inq.Close();


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
