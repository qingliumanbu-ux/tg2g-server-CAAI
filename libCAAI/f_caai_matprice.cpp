/*=========================================================================
//程序名称:     f_caai_getprice
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     获取定额成本单价
//修改人员:
//修改日期:
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
BM2_FUNCTION_EXPORT
int f_caai_getcb(CString account_period, CString price_terms, CDbConnection * conn);
int f_caai_matprice(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString sqlstr_sub = "";
	CString v_whole_backlog = "";
	CString whole_backlog = "";
	CDecimal whole_backlog_seq = 0;
	CString msc = "";
	CString prod_code = "";
	CString sg_std = "";
	CString sg_sign = "";
	CString v_ccl = "0";
	CDecimal v_price = 0;
	int v_count = 0;

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);

	try
	{
		//获取传入参数
		bcls_ret->Tables[0].Clear();
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL, "BZ_PRICE");
		bcls_ret->Tables[0].Rows.Add();

		msc = bcls_rec->Tables["BZ_PRICE"].Rows[0]["MSC"].ToString();
		whole_backlog = bcls_rec->Tables["BZ_PRICE"].Rows[0]["WHOLE_BACKLOG"].ToString();		
		whole_backlog_seq = bcls_rec->Tables["BZ_PRICE"].Rows[0]["WHOLE_BACKLOG_SEQ"].ToDecimal();
		prod_code = bcls_rec->Tables["BZ_PRICE"].Rows[0]["PROD_CODE"].ToString();
		sg_sign = bcls_rec->Tables["BZ_PRICE"].Rows[0]["SG_SIGN"].ToString();
		sg_std = bcls_rec->Tables["BZ_PRICE"].Rows[0]["SG_STD"].ToString();

		Log::Trace("", "", "msc=[{0}]whole_backlog=[{1}],whole_backlog_seq=[{2}]", msc, whole_backlog, whole_backlog_seq);

		sqlstr =  "select count(1) FROM TCAAC05B"
			" WHERE 1=1"
			" and (ALLOY_COST+GTL_COST)!=0"
			" and whole_backlog = @whole_backlog"
			" and  prod_code = @prod_code "
			" AND MSC = @msc "
			" and PRICE_TERMS = 'YJ'"	
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("msc", msc);
		cmd_inq.Parameters.Set("prod_code", prod_code);
		cmd_inq.Parameters.Set("whole_backlog", whole_backlog.Replace("111214", "").Replace("9A9B", ""));
		v_count = cmd_inq.ExecuteScalar().ToInt32();
		cmd_inq.Close();
		if (v_count == 0)//重新获取各个冶金规范的
		{
			sqlstr = " select account_period from tcaac15"
				" WHERE S_DATETIME <= @datenow"
				" AND E_DATETIME >= @datenow"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("datenow", CDateTime::Now().ToString("yyyyMMdd"));
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				doFlag = f_caai_getcb(cmd_inq.GetString(1), "YJ", conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			cmd_inq.Close();
		}

		//拼接sql语句 +BO_COST+B1_COST
		sqlstr = "";
		sqlstr_sub = "";
		v_ccl = "0";
		for (int j = 4; j < whole_backlog_seq; j ++) //全程工序代码截取
		{
			sqlstr_sub = sqlstr_sub + "+" + whole_backlog.SubstringNE(j * 2, 2) + "_COST";
			if (whole_backlog.SubstringNE(j * 2 , 2) == "B1" || whole_backlog.SubstringNE(j * 2, 2) == "B2" || whole_backlog.SubstringNE(j * 2, 2) == "B3" || whole_backlog.SubstringNE(j * 2, 2) == "D1")
			{
				v_ccl = "1";
			}
		}
		if (v_ccl != "1")
		{
			sqlstr = "SELECT ALLOY_COST+GTL_COST+A1_COST+RH_COST+QJ_COST";
		}
		else
		{
			sqlstr = "SELECT round((ALLOY_COST+GTL_COST+A1_COST+RH_COST)/ccl*100,2) + QJ_COST";
		}
		sqlstr = sqlstr + sqlstr_sub+ " FROM TCAAC05B "
			" WHERE 1=1"
			" and (ALLOY_COST+GTL_COST)!=0"
			" and whole_backlog = @whole_backlog"			
			" and  prod_code = @prod_code "
			" AND MSC = @msc "		
			" and PRICE_TERMS = 'YJ'"
			" and account_period <=@account_period"
			" order by account_period desc"
			;
		cmd_inq.SetCommandText(sqlstr);		
		cmd_inq.Parameters.Set("msc", msc);
		cmd_inq.Parameters.Set("prod_code", prod_code);
		cmd_inq.Parameters.Set("whole_backlog", whole_backlog.Replace("111214", "").Replace("9A9B", ""));
		cmd_inq.Parameters.Set("account_period", CDateTime::Now().ToString("yyyyMM"));
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			v_price = cmd_inq.GetDecimal(1).Round(2);
		}
		cmd_inq.Close();

		

		bcls_ret->Tables[0].Rows[0]["BZ_PRICE"] = v_price;

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
