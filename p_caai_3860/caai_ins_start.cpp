/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    1.0
Date:		2012-2-9
Description:
**************************************************/
//框架用头文件
#include "stdafx.h"

//-EP_CODE_VERSION 1
//-EP_SYSTEM_HEAD_BEGIN
//-此节代码请勿更改
/*<remark>=========================================================
/// <summary>
/// 成本导航
/// <para> 1. 前台CAAC13画面的F3(启动)。</para>>
/// <para> 2. 检查参数，参数可为空，空参数视为该参数不参与查询筛选，表示查询全部。对查询参数进行合理性检查。</para>
===========================================================</remark>*/

int f_caai_00(const CString& account_period, const CString& current_status, CDbConnection *conn);
int f_caai_check1(CString stats_period, CString dept_code, CString begin_time, CString end_time, CDbConnection * conn);
int f_caai_check2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_caai_gb(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_pcost(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_ft(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_sj(CString stats_period, CString dept_code, CDbConnection * conn);
int f_caai_cost(CString stats_period, CString dept_code, CString price_terms, CDbConnection * conn);
int f_caai_rate(CString stats_period, CString sub_backlog_code, CString divvy_type, CDbConnection * conn);
int f_caai_ny(CString stats_period, CString dept_code, CDbConnection * conn);
//int f_caai_11(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn) ;
// service入口
BM2F_ENTERACE(caai_ins_start)
//-EP_SYSTEM_HEAD_END

int f_caai_ins_start(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	// 数据库SQL操作字符串
	CString sqlstr = "";
	CString sqlstr1 = "";

	/* 实体类定义 */
	CModel tcaac13("TCAAC13");

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);


	int doFlag = 0;
	CModel tcaaia1("TCAAIA1");
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString		stats_period = " ";
	CString		job_code = " ";
	CString   begin_time = " ";
	CString   end_time = " ";
	CString   begin_time1 = " ";
	CString   end_time1 = " ";
	CString   dept_code = "";

	try
	{
		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString().SubstringNE(0, 8);
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString().SubstringNE(0, 8);
		tcaaia1.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		if (tcaaia1["DEPT_CODE"].ToString().Trim() == "IR")
		{
			sqlstr = "delete from tcaaia1"
				" where 1=1 "
				" and dept_code='" + tcaaia1["DEPT_CODE"].ToString().Trim() + "' "
				" and stats_period >= '" + begin_time + "'"
				" and stats_period <= '" + end_time + "'"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			//删除tcaaia12表能源数据
			sqlstr = " delete from tcaaia12 "
				" where 1=1 "
				" and prod_date>='" + begin_time + "' "
				" and prod_date<='" + end_time + "' "
				" and dept_code='" + tcaaia1["DEPT_CODE"].ToString().Trim() + "' ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			//删除tcaais2表能源数据
			sqlstr = " delete from tcaais2 "
				" where 1=1 "
				" and mat_code in(select mat_code from tcaac11 where MAT_TYPE='A') "
				" and prod_date>='" + begin_time + "' "
				" and prod_date<='" + end_time + "' "
				" and dept_code='" + tcaaia1["DEPT_CODE"].ToString().Trim() + "' ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			//铁区生产数据
			sqlstr = "insert into tcaaia1(prod_time,  prod_shift_group,prod_shift_no, dept_code, equ_no,sub_backlog_code "
				", pro_flag,sg_sign, relation_no, mat_code,wce, wt, key_seq,stats_period,rec_create_time,rec_creator) "
				" select prod_time, prod_shift_group, prod_shift_no,'IR', prod_unit_code, sub_backlog_code "
				", deal_flag,PRODUCT_CODE, ACJC_RELATION_ID, PRODUCT_CODE,SUPPLIER_CODE "
				", wt, 'MM' ||prod_time||trim(to_char(rownum, '0000000')),stats_period,@rec_create_time,@rec_creator"
				" from ("
				"  select  EVENT_DATETIME prod_time, prod_shift_group, prod_shift_no, decode(substr(prod_unit_code,0,2),'IL',' ',prod_unit_code) prod_unit_code, substr(prod_unit_code,0,2) sub_backlog_code "
				" , deal_flag, decode(substr(prod_unit_code,0,2),'IL','IL',prod_unit_code) ACJC_RELATION_ID, PRODUCT_CODE,SUPPLIER_CODE"
				" , sum(mat_act_wt) wt,EVENT_DATETIME stats_period  "
				" from tmmisac"
				" where 1=1 "
				" and PRODUCT_CODE not in (select mat_code from tcaac11 where MAT_TYPE='A') "
				" and substr(prod_unit_code,0,2) in(select sub_backlog_code from tcaac14 where code_line=@dept_code) "
				" and deal_flag in('I', 'O')"
				" and EVENT_DATETIME >= '" + begin_time + "'"
				" and EVENT_DATETIME <= '" + end_time + "'"
				" group by  EVENT_DATETIME, prod_shift_group, prod_shift_no,prod_unit_code "
				" , deal_flag, acjc_relation_id, PRODUCT_CODE,SUPPLIER_CODE"
				"  having sum(mat_act_wt) != 0"
				" )"
				;
			//Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", tcaaia1["DEPT_CODE"].ToString());
			cmd_inq.Parameters.Set("rec_create_time", datenow);
			cmd_inq.Parameters.Set("rec_creator", s.userid);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " select distinct MEASURE_MODE,mat_code from tcaac07  "
				" where 1=1 "
				" and mat_code in(select mat_code from tcaac11 where MAT_TYPE='A') "
				" and back_code_1='" + tcaaia1["DEPT_CODE"].ToString().Trim() + "' ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				if (cmd_inq.GetString(1) == "7")
				{
					//铁区能源数据,如果按系数分摊则插入tcaaia12表
					sqlstr1 = " insert into TCAAIA12(rec_create_time,rec_creator,dept_code,prod_date,shift_no,shift_group,equ_no,mat_code,mat_name,sub_backlog_code,comsume_wt) "
						" select @rec_create_time,@rec_creator,@dept_code,EVENT_DATETIME,prod_shift_no,prod_shift_group,prod_unit_code,product_code,mat_name,substr(prod_unit_code,0,2),case when  product_code='001923' and substr(prod_unit_code,0,2)='BF' then sum(mat_act_wt)*-1 else sum(mat_act_wt) end  "
						" from tmmisac t1 "
						" left join tcaac11 t2 on t1.product_code=t2.mat_code "
						" where 1=1 "
						" and PRODUCT_CODE =@mat_code "
						" and substr(prod_unit_code,0,2) in(select sub_backlog_code from tcaac14 where code_line=@dept_code) "
						" and deal_flag in('I', 'O')"
						" and mat_act_wt!=0 "
						" and EVENT_DATETIME >= '" + begin_time + "'"
						" and EVENT_DATETIME <= '" + end_time + "'";
					cmd_inq_1.SetCommandText(sqlstr1);
					//Log::Trace("", "", "sqlstr11={0}", sqlstr1);
					cmd_inq_1.Parameters.Set("dept_code", tcaaia1["DEPT_CODE"].ToString());
					cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("rec_create_time", datenow);
					cmd_inq_1.Parameters.Set("rec_creator", s.userid);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
				else
				{
					//铁区能源数据，如果不是按系数分摊则插入tcaais2表
					sqlstr1 = " insert into tcaais2(rec_create_time,rec_creator,dept_code,stats_period,prod_date,prod_shift_no,prod_shift_group,DIVVY_TYPE,equ_no,mat_code,mat_name,sub_backlog_code,wt,RULE_TYPE) "
						" select @rec_create_time,@rec_creator,@dept_code,EVENT_DATETIME,EVENT_DATETIME,prod_shift_no,prod_shift_group,@DIVVY_TYPE,decode(prod_unit_code,'IL0',' ',prod_unit_code),product_code,mat_name,substr(prod_unit_code,0,2),case when  product_code='001923' and substr(prod_unit_code,0,2)='BF' then sum(mat_act_wt)*-1 else sum(mat_act_wt) end,'2'  "
						" from tmmisac t1 "
						" left join tcaac11 t2 on t1.product_code=t2.mat_code "
						" where 1=1 "
						" and PRODUCT_CODE =@mat_code "
						" and substr(prod_unit_code,0,2) in(select sub_backlog_code from tcaac14 where code_line=@dept_code) "
						" and deal_flag in('I', 'O')"
						" and mat_act_wt!=0 "
						" and EVENT_DATETIME >= '" + begin_time + "'"
						" and EVENT_DATETIME <= '" + end_time + "'"
						" group by EVENT_DATETIME,prod_shift_no,prod_shift_group,prod_unit_code,product_code,mat_name,substr(prod_unit_code,0,2) ";
					cmd_inq_1.SetCommandText(sqlstr1);
					cmd_inq_1.Parameters.Set("dept_code", tcaaia1["DEPT_CODE"].ToString());
					cmd_inq_1.Parameters.Set("mat_code", cmd_inq.GetString(2));
					cmd_inq_1.Parameters.Set("DIVVY_TYPE", cmd_inq.GetString(1));
					cmd_inq_1.Parameters.Set("rec_create_time", datenow);
					cmd_inq_1.Parameters.Set("rec_creator", s.userid);
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
			}
			cmd_inq.Close();


		}
		//更新物料名称
		sqlstr = " update tcaaia1 t1 set mat_name=(select mat_name from tcaac11 t2 where t1.mat_code=t2.mat_code) "
			" where 1=1"
			" and exists (select 1 from tcaac11 t2 where t1.mat_code=t2.mat_code)"
			" and  dept_code='" + tcaaia1["DEPT_CODE"].ToString().Trim() + "' and  stats_period>='" + begin_time + "' and  stats_period<='" + end_time + "' "
			;

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句		

			sqlstr = " SELECT stats_period	FROM tcaac06 where substr(E_DATETIME,0,8)>='" + begin_time + "' and substr(E_DATETIME,0,8)<='" + end_time + "' ";
			break;
		}
		cmd_inq.SetCommandText(sqlstr);// 设置执行的SQL语句
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			stats_period = cmd_inq.GetString(1);
			EIClass bcls_rec1;
			EIClass bcls_ret1;

			bcls_rec1.Tables[0].Columns.Add(DT_STRING, "STATS_PERIOD");
			bcls_rec1.Tables[0].Columns.Add(DT_STRING, "BEGIN_TIME");
			bcls_rec1.Tables[0].Columns.Add(DT_STRING, "END_TIME");
			bcls_rec1.Tables[0].Columns.Add(DT_STRING, "DEPT_CODE");
			bcls_rec1.Tables[0].Rows.Add();
			bcls_rec1.Tables[0].Rows[0]["STATS_PERIOD"] = stats_period;
			bcls_rec1.Tables[0].Rows[0]["BEGIN_TIME"] = begin_time;
			bcls_rec1.Tables[0].Rows[0]["END_TIME"] = end_time;
			bcls_rec1.Tables[0].Rows[0]["DEPT_CODE"] = tcaaia1["DEPT_CODE"].ToString().Trim();
			
			sqlstr = " SELECT S_DATETIME,E_DATETIME	FROM tcaac06 WHERE STATS_PERIOD = '" + stats_period + "'	";
			cmd_inq_sub.SetCommandText(sqlstr);
			cmd_inq_sub.ExecuteReader();
			while (cmd_inq_sub.Read())
			{
				begin_time1 = cmd_inq_sub.GetString(1);
				end_time1 = cmd_inq_sub.GetString(2);
			}
			cmd_inq_sub.Close();

			sqlstr = " select * from tcaac13 where dept_code='" + tcaaia1["DEPT_CODE"].ToString().Trim() + "' and stats_period='" + stats_period + "' order by job_code ";
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.ExecuteReader();
			while (cmd_inq_1.Read())
			{
				cmd_inq_1.Fetch(tcaac13);
				tcaac13["STATS_PERIOD"] = stats_period;
				tcaac13["DEPT_CODE"] = tcaaia1["DEPT_CODE"].ToString().Trim();
				//开始核算
				if (tcaac13["JOB_CODE"].ToString() == "00")
				{
					tcaac13["JOB_CODE"] = "00";
					doFlag = f_caai_00(stats_period, "1", conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}
				//生产实绩数据处理
				if (tcaac13["JOB_CODE"].ToString() == "01")
				{
					tcaac13["JOB_CODE"] = "01";
					doFlag = f_caai_check1(stats_period, tcaaia1["DEPT_CODE"].ToString().Trim(), begin_time1, end_time1, conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}
				//产副品数据检索及处理
				if (tcaac13["JOB_CODE"].ToString() == "02")
				{
					doFlag = f_caai_check2(&bcls_rec1, &bcls_ret1, conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}
				//工序产出核算
				if (tcaac13["JOB_CODE"].ToString() == "11")
				{
					doFlag = f_caai_gb(stats_period, tcaaia1["DEPT_CODE"].ToString().Trim(), conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}

				//工序消耗项核算
				if (tcaac13["JOB_CODE"].ToString() == "12")
				{
					doFlag = f_caai_sj(stats_period, tcaaia1["DEPT_CODE"].ToString().Trim(), conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}

				if (tcaac13["JOB_CODE"].ToString() == "20")
				{
					//其他方式的分摊方法
// DM8 适配 CHANGE-264:查询。空值搜索 DECODE 改为标准 CASE。
// 改写原因：空值搜索 DECODE 改为标准 CASE,不依赖 NULL 相等匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
					// sqlstr = " select distinct sub_backlog_code,divvy_type"
						// " from tcaais2"
						// " where 1=1"
						// " AND dept_code = decode(trim(@dept_code), '', sub_backlog_code, @dept_code)"
						// " and STATS_PERIOD = @stats_period"
						// ;
// DM8 SQL：
					sqlstr = " select distinct sub_backlog_code,divvy_type"
						" from tcaais2"
						" where 1=1"
						" AND dept_code = CASE WHEN trim(@dept_code) IS NULL OR trim(@dept_code) = '' THEN sub_backlog_code ELSE @dept_code END"
						" and STATS_PERIOD = @stats_period"
						;
					cmd_inq_sub.SetCommandText(sqlstr);
					cmd_inq_sub.Parameters.Set("dept_code", tcaaia1["DEPT_CODE"].ToString().Trim());
					cmd_inq_sub.Parameters.Set("stats_period", stats_period);
					cmd_inq_sub.ExecuteReader();
					while (cmd_inq_sub.Read())
					{
						doFlag = f_caai_rate(stats_period, cmd_inq_sub.GetString(1), cmd_inq_sub.GetString(2), conn);
						if (doFlag != 0)
						{
							throw CApplicationException(-1, s.msg, log.Location);
						}

					}
					cmd_inq_sub.Close();
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}

				if (tcaac13["JOB_CODE"].ToString() == "21")
				{
					doFlag = f_caai_ft(stats_period, tcaaia1["DEPT_CODE"].ToString().Trim(), conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}
				if (tcaac13["JOB_CODE"].ToString() == "22")
				{
					doFlag = f_caai_ny(stats_period, tcaaia1["DEPT_CODE"].ToString().Trim(), conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}

				if (tcaac13["JOB_CODE"].ToString() == "30")
				{
					doFlag = f_caai_cost(stats_period, tcaaia1["DEPT_CODE"].ToString().Trim(), "PJ", conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}

				if (tcaac13["JOB_CODE"].ToString() == "31")
				{
					doFlag = f_caai_pcost(stats_period, tcaaia1["DEPT_CODE"].ToString().Trim(), conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}

				//结束核算
				if (tcaac13["JOB_CODE"].ToString() == "40")
				{
					tcaac13["JOB_CODE"] = "40";
					doFlag = f_caai_00(stats_period, "1", conn);
					if (doFlag != 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tcaac13["SUCCS_FLAG"] = "1";
					tcaac13.Update("SUCCS_FLAG", "JOB_CODE,DEPT_CODE,STATS_PERIOD");
				}
			}
			cmd_inq_1.Close();
			
		}
		cmd_inq.Close();
		

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}